// SPDX-License-Identifier: BSD-3-Clause
//
// LocalGraphIsta -- thin C++ facade over the local-graph incremental timing
// engine (LocalSta + PtGraph) extracted from the lrf module.
//
// The ISTA exposes two API surfaces and deliberately owns NEITHER scheduling
// NOR traversal order:
//   * compute -- build a per-instance local timing graph and propagate
//                delay / arrival / required through it; what-if cell swap.
//   * write   -- commit a local result back onto the global sta::Graph.
// Deciding which instances to visit, in what order, with what parallelism,
// and when to commit is the optimizer's job. Build your optimizer on top.
//
// Lifecycle: construct with a fully-initialized sta::dbSta (design + liberty
// loaded), call prepare() once, then analyzeInstance()/commitToGlobal().
// Returned PtGraphs are owned by the underlying LocalSta.

#pragma once

#include <cstddef>
#include <string>

namespace sta {
class dbSta;
class Instance;
class ArcDelayCalc;
}  // namespace sta

namespace lgista {

class LocalSta;
class PtGraph;

class LocalGraphIsta {
 public:
  explicit LocalGraphIsta(sta::dbSta* sta);
  ~LocalGraphIsta();

  LocalGraphIsta(const LocalGraphIsta&) = delete;
  LocalGraphIsta& operator=(const LocalGraphIsta&) = delete;

  // Bring the global timing graph up to date (arrivals + requireds). Safe to
  // call multiple times; cheap when nothing changed.
  void prepare();

  // Compute: build the local timing graph around one instance and propagate
  // delay / arrival / required through it. Returns the local graph (owned by
  // the engine) or nullptr if the instance has no usable driver. When
  // print_report is true a per-vertex timing dump is written to stdout.
  PtGraph* analyzeInstance(sta::Instance* inst, bool print_report = true);

  // Convenience: look the instance up by full hierarchical name first.
  PtGraph* analyzeInstance(const std::string& inst_name,
                           bool print_report = true);

  // Write API: commit a local graph's slew + arrival/required back onto the
  // global sta::Graph. Call after analyzeInstance() (or your own compute) when
  // you want the local result reflected globally. WHEN and in WHAT ORDER to
  // commit across instances is the optimizer's decision, not the ISTA's.
  void commitToGlobal(PtGraph* pt_graph);

  // Build a stack-allocated PtGraph around one instance, compute local
  // delays/arrivals/requireds, and write back to the global graph.  Does
  // NOT call prepare() (no full-graph updateTiming) — the caller is
  // responsible for the global graph being up to date.  Returns false if
  // the instance has no usable driver.
  bool analyzeAndCommit(sta::Instance* inst);

  // DEMO/convenience only: iterate every leaf instance and analyze it. This
  // imposes an iteration order, which is normally the optimizer's job -- use
  // it for smoke tests, not as the production traversal. Returns the number
  // of instances for which a local graph was built.
  size_t analyzeAll(bool print_report = false);

  LocalSta* localSta() { return local_sta_; }

 private:
  sta::dbSta* sta_ = nullptr;
  LocalSta* local_sta_ = nullptr;
  sta::ArcDelayCalc* arc_delay_calc_ = nullptr;
  bool prepared_ = false;

  // Lightweight init that creates LocalSta + ArcDelayCalc WITHOUT calling
  // updateTiming(true).  The global graph must already be built/levelized.
  void ensureInit();
};

}  // namespace lgista
