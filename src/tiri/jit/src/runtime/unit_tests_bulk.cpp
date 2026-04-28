// Unit tests for bulk TValue operations.
// Copyright (C) 2026 Paul Manias

#include <kotuku/main.h>

#ifdef ENABLE_UNIT_TESTS

#include "lj_obj.h"
#include "lj_bulk.h"

#include <array>
#include <vector>

namespace {

struct TestCase {
   const char* name;
   bool (*fn)(kt::Log &Log);
};

constexpr std::array<MSize, 13> glBulkCounts = { {
   0, 1, 2, 3, 4, 15, 16, 31, 32, 63, 64, 65, 129
} };

static void fill_slots(std::vector<TValue> &Slots)
{
   for (MSize i = 0; i < MSize(Slots.size()); i++) {
      Slots[i].u64 = U64x(5a5a0000, 00000000) + uint64_t(i);
   }
}

static bool check_range(const TValue *Actual, const std::vector<uint64_t> &Expected, kt::Log &Log, CSTRING Label)
{
   for (MSize i = 0; i < MSize(Expected.size()); i++) {
      if (not (Actual[i].u64 IS Expected[i])) {
         Log.error("%s mismatch at slot %d: got %.16llx expected %.16llx",
            Label, int(i), (unsigned long long)Actual[i].u64, (unsigned long long)Expected[i]);
         return false;
      }
   }
   return true;
}

static bool test_bulk_nil(kt::Log &Log)
{
   for (MSize Count : glBulkCounts) {
      std::vector<TValue> slots(Count + 8);
      fill_slots(slots);
      TValue *dst = slots.data() + 1;

      lj_bulk_nil_tvalue(dst, Count);

      for (MSize i = 0; i < Count; i++) {
         if (not (dst[i].it64 IS -1)) {
            Log.error("nil fill failed for count %d at slot %d", int(Count), int(i));
            return false;
         }
      }
      if (not (slots[0].u64 IS U64x(5a5a0000, 00000000))) {
         Log.error("nil fill overwrote prefix guard for count %d", int(Count));
         return false;
      }
   }
   return true;
}

static bool test_bulk_copy(kt::Log &Log)
{
   for (MSize Count : glBulkCounts) {
      std::vector<TValue> src_slots(Count + 8);
      std::vector<TValue> dst_slots(Count + 8);
      fill_slots(src_slots);
      fill_slots(dst_slots);

      TValue *src = src_slots.data() + 1;
      TValue *dst = dst_slots.data() + 3;
      std::vector<uint64_t> expected(Count);
      for (MSize i = 0; i < Count; i++) expected[i] = src[i].u64;

      lj_bulk_copy_tvalue(dst, src, Count);
      if (not check_range(dst, expected, Log, "copy")) return false;
   }
   return true;
}

static bool test_bulk_move_overlap_forward(kt::Log &Log)
{
   for (MSize Count : glBulkCounts) {
      std::vector<TValue> slots(Count + 12);
      fill_slots(slots);

      TValue *src = slots.data() + 1;
      TValue *dst = slots.data() + 3;
      std::vector<uint64_t> expected(Count);
      for (MSize i = 0; i < Count; i++) expected[i] = src[i].u64;

      lj_bulk_move_tvalue(dst, src, Count);
      if (not check_range(dst, expected, Log, "forward overlap move")) return false;
   }
   return true;
}

static bool test_bulk_move_overlap_backward(kt::Log &Log)
{
   for (MSize Count : glBulkCounts) {
      std::vector<TValue> slots(Count + 12);
      fill_slots(slots);

      TValue *src = slots.data() + 3;
      TValue *dst = slots.data() + 1;
      std::vector<uint64_t> expected(Count);
      for (MSize i = 0; i < Count; i++) expected[i] = src[i].u64;

      lj_bulk_move_tvalue(dst, src, Count);
      if (not check_range(dst, expected, Log, "backward overlap move")) return false;
   }
   return true;
}

} // namespace

void bulk_unit_tests(int &Passed, int &Total)
{
   constexpr std::array<TestCase, 4> Tests = { {
      { "bulk_nil", test_bulk_nil },
      { "bulk_copy", test_bulk_copy },
      { "bulk_move_overlap_forward", test_bulk_move_overlap_forward },
      { "bulk_move_overlap_backward", test_bulk_move_overlap_backward }
   } };

   for (const TestCase& Test : Tests) {
      kt::Log Log("BulkTests");
      Log.branch("Running %s", Test.name);
      ++Total;
      if (Test.fn(Log)) {
         ++Passed;
         Log.msg("%s passed", Test.name);
      }
      else {
         Log.error("%s failed", Test.name);
      }
   }
}

#endif // ENABLE_UNIT_TESTS

