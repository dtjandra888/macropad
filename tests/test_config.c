#include "unity.h"

#include "config.h"
#include "fake_flash.h"

#include <string.h>

void setUp(void) { fake_flash_reset(); }
void tearDown(void) {}

static Config sample_config(void) {
  Config c;
  memset(&c, 0, sizeof(c)); // zero padding so the CRC is deterministic
  c.macro_count = 2;
  c.macros[0].length = 1;
  c.macros[0].mode = MACRO_MODE_HOLD;
  c.macros[0].strokes[0] = (KeyStroke){.key = 0x04, .modifier = 0x02};
  c.macros[1].length = 2;
  c.macros[1].strokes[0] = (KeyStroke){.key = 0x06, .modifier = 0x01};
  c.macros[1].strokes[1] = (KeyStroke){.key = 0x19, .modifier = 0x01};
  return c;
}

static ConfigStorage valid_storage(void) {
  ConfigStorage s;
  memset(&s, 0, sizeof(s));
  s.config = sample_config();
  s.header.magic = CONFIG_MAGIC;
  s.header.size = sizeof(Config);
  s.header.crc32 = crc32((const uint8_t *)&s.config, sizeof(Config));
  return s;
}

// --- crc32 ---

static void test_crc32_standard_check_value(void) {
  // The standard CRC-32 (IEEE 802.3) check value
  const char *data = "123456789";
  TEST_ASSERT_EQUAL_HEX32(0xCBF43926u, crc32((const uint8_t *)data, 9));
}

static void test_crc32_empty_input_is_zero(void) {
  TEST_ASSERT_EQUAL_HEX32(0x00000000u, crc32((const uint8_t *)"", 0));
}

static void test_crc32_known_vectors(void) {
  TEST_ASSERT_EQUAL_HEX32(0xE8B7BE43u, crc32((const uint8_t *)"a", 1));
  const char *fox = "The quick brown fox jumps over the lazy dog";
  TEST_ASSERT_EQUAL_HEX32(0x414FA339u,
                          crc32((const uint8_t *)fox, strlen(fox)));
}

static void test_crc32_detects_single_bit_flip(void) {
  uint8_t data[32];
  for (size_t i = 0; i < sizeof(data); i++) data[i] = (uint8_t)i;
  uint32_t before = crc32(data, sizeof(data));
  data[17] ^= 0x08;
  TEST_ASSERT_NOT_EQUAL(before, crc32(data, sizeof(data)));
}

// --- config_is_valid ---

static void test_valid_storage_passes(void) {
  ConfigStorage s = valid_storage();
  TEST_ASSERT_TRUE(config_is_valid(&s));
}

static void test_wrong_magic_fails(void) {
  ConfigStorage s = valid_storage();
  s.header.magic = 0xDEADBEEF;
  TEST_ASSERT_FALSE(config_is_valid(&s));
}

static void test_wrong_size_fails(void) {
  ConfigStorage s = valid_storage();
  s.header.size = sizeof(Config) - 1;
  TEST_ASSERT_FALSE(config_is_valid(&s));
}

static void test_corrupted_config_fails(void) {
  ConfigStorage s = valid_storage();
  s.config.macros[1].strokes[1].key ^= 0x01;
  TEST_ASSERT_FALSE(config_is_valid(&s));
}

static void test_wrong_crc_fails(void) {
  ConfigStorage s = valid_storage();
  s.header.crc32 ^= 0x80000000u;
  TEST_ASSERT_FALSE(config_is_valid(&s));
}

static void test_erased_flash_is_invalid(void) {
  ConfigStorage s;
  memset(&s, 0xFF, sizeof(s));
  TEST_ASSERT_FALSE(config_is_valid(&s));
}

// --- save_config / load_config (flash faked as RAM) ---

static void test_storage_fits_in_one_flash_page(void) {
  // save_config only programs a single page
  TEST_ASSERT_LESS_OR_EQUAL_size_t(FLASH_PAGE_SIZE, sizeof(ConfigStorage));
}

static void test_save_writes_last_sector_with_interrupts_disabled(void) {
  ConfigStorage s = valid_storage();
  save_config(&s);

  TEST_ASSERT_EQUAL_INT(1, fake_flash_erase_count);
  TEST_ASSERT_EQUAL_UINT32(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE,
                           fake_flash_last_erase_offset);
  TEST_ASSERT_EQUAL_size_t(FLASH_SECTOR_SIZE, fake_flash_last_erase_size);

  TEST_ASSERT_EQUAL_INT(1, fake_flash_program_count);
  TEST_ASSERT_EQUAL_UINT32(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE,
                           fake_flash_last_program_offset);
  TEST_ASSERT_EQUAL_size_t(FLASH_PAGE_SIZE, fake_flash_last_program_size);

  TEST_ASSERT_TRUE(fake_flash_interrupts_were_disabled);
}

static void test_save_then_load_round_trips(void) {
  ConfigStorage saved = valid_storage();
  save_config(&saved);

  ConfigStorage loaded;
  memset(&loaded, 0, sizeof(loaded));
  load_config(&loaded);

  TEST_ASSERT_EQUAL_MEMORY(&saved, &loaded, sizeof(ConfigStorage));
  TEST_ASSERT_TRUE(config_is_valid(&loaded));
}

static void test_save_pads_rest_of_page_with_erased_bytes(void) {
  ConfigStorage s = valid_storage();
  save_config(&s);

  const uint8_t *page =
      fake_flash_memory + PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;
  for (size_t i = sizeof(ConfigStorage); i < FLASH_PAGE_SIZE; i++) {
    TEST_ASSERT_EQUAL_HEX8(0xFF, page[i]);
  }
}

static void test_save_overwrites_previous_config(void) {
  ConfigStorage first = valid_storage();
  save_config(&first);

  ConfigStorage second = valid_storage();
  second.config.macros[0].strokes[0].key = 0x2C;
  second.header.crc32 =
      crc32((const uint8_t *)&second.config, sizeof(Config));
  save_config(&second);

  ConfigStorage loaded;
  load_config(&loaded);
  TEST_ASSERT_TRUE(config_is_valid(&loaded));
  TEST_ASSERT_EQUAL_HEX8(0x2C, loaded.config.macros[0].strokes[0].key);
}

static void test_load_from_erased_flash_is_invalid(void) {
  ConfigStorage loaded;
  load_config(&loaded);
  TEST_ASSERT_FALSE(config_is_valid(&loaded));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_crc32_standard_check_value);
  RUN_TEST(test_crc32_empty_input_is_zero);
  RUN_TEST(test_crc32_known_vectors);
  RUN_TEST(test_crc32_detects_single_bit_flip);
  RUN_TEST(test_valid_storage_passes);
  RUN_TEST(test_wrong_magic_fails);
  RUN_TEST(test_wrong_size_fails);
  RUN_TEST(test_corrupted_config_fails);
  RUN_TEST(test_wrong_crc_fails);
  RUN_TEST(test_erased_flash_is_invalid);
  RUN_TEST(test_storage_fits_in_one_flash_page);
  RUN_TEST(test_save_writes_last_sector_with_interrupts_disabled);
  RUN_TEST(test_save_then_load_round_trips);
  RUN_TEST(test_save_pads_rest_of_page_with_erased_bytes);
  RUN_TEST(test_save_overwrites_previous_config);
  RUN_TEST(test_load_from_erased_flash_is_invalid);
  return UNITY_END();
}
