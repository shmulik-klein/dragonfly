// Copyright 2022, Roman Gershman.  All rights reserved.
// See LICENSE for licensing terms.
//

#pragma once

#include <bitset>

#include "io/file.h"
#include "server/table.h"
#include "util/fibers/simple_channel.h"

namespace dfly {

class RdbSerializer;

class SliceSnapshot {
 public:
  using StringChannel =
      ::util::fibers_ext::SimpleChannel<std::string, base::mpmc_bounded_queue<std::string>>;

  SliceSnapshot(PrimeTable* prime, ExpireTable* et, StringChannel* dest);
  ~SliceSnapshot();

  void Start(uint64_t version);
  void Join();

  uint64_t snapshot_version() const {
    return snapshot_version_;
  }

 private:
  void FiberFunc();
  bool FlushSfile(bool force);
  void SerializeCb(MainIterator it);
  bool SaveCb(MainIterator it);

  ::boost::fibers::fiber fb_;

  enum {PHYSICAL_LEN = 128};
  std::bitset<PHYSICAL_LEN> physical_mask_;

  std::unique_ptr<io::StringFile> sfile_;
  std::unique_ptr<RdbSerializer> rdb_serializer_;

  // version upper bound for entries that should be saved (not included).
  uint64_t snapshot_version_ = 0;
  PrimeTable* prime_table_;
  StringChannel* dest_;

  size_t serialized_ = 0, skipped_ = 0;
};

}  // namespace dfly